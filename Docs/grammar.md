$$
\begin{align}
[\text{Prog}] & \to [\text{Stmt}]^*\\
[\text{Stmt}] & \to
\begin{cases}
exit([Expr]);\\
let\space\text{ident} = [Expr];
\end{cases}\\
[\text{Expr}] & \to
\begin{cases}
[\text{Term}]\\
[\text{BinExpr}]
\end{cases}\\
[\text{BinExpr}] & \to
\begin{cases}
[\text{Expr}] * [\text{Expr}] & prio = 1\\
[\text{Expr}] + [\text{Expr}] & prio = 0\\
\end{cases}\\
[\text{Term}] & \to
\begin{cases}
\text{int_lit}\\
\text{ident}\\
([\text{Expr}])
\end{cases}\\
\end{align}
$$